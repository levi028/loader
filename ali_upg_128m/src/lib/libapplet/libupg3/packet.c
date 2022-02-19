 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: packet.c
*    Description: implement functions interface about packet
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

#include "packet.h"
#include "error_code.h"
#include "upgrade_command.h"

#define LIB_ASH_OC(a)           sci_write(p2p_uart_id, a)
#define LIB_ASH_IC()            sci_read(p2p_uart_id)

#if 0//def _DEBUG_VERSION_
#define DEBUG
#define SH_PRINTF   soc_printf
#else
#define SH_PRINTF(...)
#endif

static unsigned char l_packet_head_flag[5] = "HEAD";
static unsigned char l_packet_comtest[7] = "comtest";
static  PACKET  send_packet;

void com_clear_buffer(void)
{
    sci_clear_buff(p2p_uart_id);
}
UINT32 com_send_char(unsigned char c)
{
    return com_send(&c, 1);
}
 UINT32 com_send(unsigned char *buffer, UINT32 n_len)
{
    UINT32 i = 0;

    if(!buffer)
    {
        return 0;
    }
    for(i = 0; i < n_len; i++)
    {
        sci_write(p2p_uart_id, buffer[i]);
    }
    return  i ;
}
 UINT32 com_read(unsigned char *buffer, UINT32 n_len)
{
    UINT32 n_return = SUCCESS;
    UINT32 i = 0;

    if(!buffer)
    {
        return !SUCCESS;
    }
    for(i = 0; i < n_len; i++)
    {
        if(SUCCESS != sci_read_tm(p2p_uart_id, buffer+i, 1))
        {
            n_return = ERROR_READDATA_TIMEOUT;
            break;
        }
    }
    return n_return;
}
UINT32 com_read_tm(unsigned char *buffer, UINT32 n_len, UINT32 n_time_out)
{
    UINT32 n_return = SUCCESS;
    UINT32 i = 0;

    if(!buffer)
    {
        return !SUCCESS;
    }
    for(i = 0; i < n_len; i++)
    {
        if(SUCCESS != sci_read_tm(p2p_uart_id, buffer+i, n_time_out * 1000))
        {
            n_return = ERROR_READDATA_TIMEOUT;
            break;
        }
    }
    return n_return;
}

UINT32 com_send2back_tm(unsigned char *buffer, UINT32 n_len, UINT32 n_time_out)
{
    UINT32 i = 0;
    INT8 tmp = 0;

    if(!buffer)
    {
        return !SUCCESS;
    }
    for(i=0;i<n_len;i++)
    {
        sci_write(p2p_uart_id, buffer[i]);

        if(sci_read_tm(p2p_uart_id, (UINT8 *)&tmp, n_time_out * 1000)!=SUCCESS)
        {
            return !SUCCESS;
        }
        if(tmp!= buffer[i])
        {
            return !SUCCESS;
        }
    }
    return SUCCESS;
}

/****************************************************************************
 *  Name:       make_packet_head_buffer
 *  Function:   make packet head buffer
 *  Remark:     local function
 ****************************************************************************/
static void make_packet_head_buffer(PPACKET p_packet, unsigned char *packet_head)
{
    UINT32 n_crc = 0;

    if ((NULL == p_packet) || (NULL == packet_head))
    {
        return;
    }
    MEMSET(packet_head, 0, PACKET_HEAD_LEN);
    // set packet head information;
    MEMCPY(packet_head + PACKET_HEAD_FLAG_OFFSET,  l_packet_head_flag, sizeof(l_packet_head_flag) - 1);
    MEMCPY(packet_head + PACKET_TYPE_OFFSET, &(p_packet->packet_type), sizeof(p_packet->packet_type));
    MEMCPY(packet_head + PACKET_BLANKFLAG_OFFSET, &(p_packet->blank_flag), sizeof(p_packet->blank_flag));
    store_long(packet_head + PACKET_INDEX_OFFSET, p_packet->packet_index);
    MEMCPY(packet_head + PACKET_REPEAT_OFFSET, &(p_packet->packet_repeat), sizeof(p_packet->packet_repeat));
    store_long( packet_head + PACKET_LENGTH_OFFSET, p_packet->packet_length);
    n_crc = mg_table_driven_crc(0xFFFFFFFF, packet_head + PACKET_TYPE_OFFSET, PACKET_HEAD_LEN - 8);
    store_long( packet_head + PACKET_HEAD_CRC_OFFSET, n_crc);
}

/****************************************************************************
 *  Name:       make_packet_head
 *  Function:   make packet head
 *  Remark:     local function
 ****************************************************************************/
static void make_packet_head(unsigned char *packet_head, PPACKET p_packet)
{
    if ((NULL == packet_head) || (NULL == p_packet))
    {
        return;
    }
    MEMSET(p_packet, 0, sizeof(PACKET));
    // set packet head information;
    p_packet->packet_type = packet_head[PACKET_TYPE_OFFSET];
    p_packet->blank_flag = packet_head[PACKET_BLANKFLAG_OFFSET];
    p_packet->packet_index = fetch_long(packet_head + PACKET_INDEX_OFFSET);
    p_packet->packet_repeat = packet_head[PACKET_REPEAT_OFFSET];
    p_packet->packet_length = fetch_long(packet_head + PACKET_LENGTH_OFFSET);

}

/****************************************************************************
 *  Name:       packet_send
 *  Function:   send a packet
 *  variable:   p_packet     -- packet information point
 ****************************************************************************/
UINT32 packet_send(PPACKET p_packet)
{
    //create packet head buffer;
    unsigned char packet_head[PACKET_HEAD_LEN] = {0};
    unsigned char sz_crc[4] = {0};
    UINT32 n_length = 0;
    UINT32 n_crc = 0;
    UINT32 tmp_magic_four = 4;

    if (NULL == p_packet)
    {
        return GENERIC_ERROR;
    }
    make_packet_head_buffer(p_packet, packet_head);
    //send packet header
    if(PACKET_HEAD_LEN != com_send(packet_head, PACKET_HEAD_LEN))
    {
        return ERROR_SENDPACKETHEAD;
    }
    n_length = p_packet->packet_length;
    if((0 != p_packet->blank_flag) && (PACKET_DATA == p_packet->packet_type))
    {
        n_length = 4;
    }
    if(n_length != com_send(p_packet->data_buffer, n_length))
    {
        return ERROR_SENDPACKETDATA;
    }
    n_crc = mg_table_driven_crc(0xFFFFFFFF, p_packet->data_buffer, n_length);
    store_long(sz_crc, n_crc);
    if(tmp_magic_four != com_send(sz_crc, tmp_magic_four))
    {
        return ERROR_SENDPACKETCRC;
    }
    return SUCCESS;
}

/****************************************************************************
 *  Name:       packet_receive
 *  Function:   receive a packet
 *  variable:   p_packet     -- packet information point
 *              n_time_out    -- time out
 ****************************************************************************/
UINT32 packet_receive(PPACKET p_packet, UINT32 n_time_out)
{
    //receive and compare packet head flag
    unsigned char c = 0;
    BOOL b_packet_head = FALSE;
    BOOL b_comtest = FALSE;
    UINT32 i = 0;
    UINT32 compare_base = 128;
    int n = -1;
    int n_base = 6;
    int n_base_third = 3;
    unsigned char packet_head[PACKET_HEAD_LEN] = {0};
    UINT32 n_crc = 0;

    if (NULL == p_packet)
    {
        return GENERIC_ERROR;
    }
    MEMSET(p_packet, 0, sizeof(PACKET));
    i = 0;
    while(i < n_time_out)
    {
        if('H' == c)
        {
            // judge if packet head
            for(n = 0; n < 3; n++)
            {
                if(SUCCESS != com_read_tm(&c, 1, 1))
                {
                    c = 0;
                    break;
                }
                if(c != l_packet_head_flag[n + 1])
                {
                    break;
                }
            }

            if(n_base_third == n)
            {
                b_packet_head = TRUE;
                break;
            }
        }
        else if('c' == c)
        {
            p_packet->data_buffer[0] = 'c';
            //judge if comtest
            for(n = 0; n < 6; n++)
            {
        //      c = LIB_ASH_IC();
                if(SUCCESS != com_read_tm(&c, 1, 1))
                {
                    c = 0;
                    break;
                }
                if(c != l_packet_comtest[n + 1])
                {
                    break;
                }
            }
            if(n_base == n)
            {
                b_comtest = TRUE;
                break;
            }
        }
        else if(SUCCESS != com_read(&c, 1))
        {
            sleep(1);
            i++;
        }
    }

    if(b_packet_head)
    {
        //receive packet head
        if(SUCCESS != com_read_tm(packet_head + PACKET_TYPE_OFFSET, PACKET_HEAD_LEN - 4, 1000))
        {
            SH_PRINTF("ERROR:receive head error");
            return ERROR_PACKET_RECEIVEHEAD;
        }
        //compare CRC about packet head
        n_crc = mg_table_driven_crc(0xFFFFFFFF, packet_head + PACKET_TYPE_OFFSET, PACKET_HEAD_LEN - 8);
        if(n_crc != fetch_long(packet_head + PACKET_HEAD_CRC_OFFSET))
        {
            return ERROR_PACKET_HEADCRC;
        }
        make_packet_head(packet_head, p_packet);
        //Receive data and CRC
        if(SUCCESS != com_read_tm(p_packet->data_buffer, p_packet->packet_length + 4, 1000))
        {
            return ERROR_PACKET_RECEIVEDATA;
        }
        n_crc = mg_table_driven_crc(0xFFFFFFFF, p_packet->data_buffer, p_packet->packet_length);
        if(n_crc != fetch_long(p_packet->data_buffer + p_packet->packet_length))
        {
            return ERROR_PACKET_DATACRC;
        }
    }
    else if(b_comtest)
    {
        p_packet->packet_type = PACKET_COMMAND;
        MEMCPY(p_packet->data_buffer, l_packet_comtest, 7);
        i = 7;
        while(i < compare_base)
        {
            p_packet->data_buffer[i] = LIB_ASH_IC();
            c = p_packet->data_buffer[i];
            if ((0x0D == c) || (0x0A == c))
            {
                    break;
            }
            i++;
        }
        p_packet->packet_length = STRLEN((const char *)(p_packet->data_buffer));
        for(i = 0; i < p_packet->packet_length; i++)
        {
            LIB_ASH_OC(p_packet->data_buffer[i]);
        }
        SH_PRINTF(p_packet->data_buffer);
        g_packet_index = 0;
        g_packet_repeat = 0;
    }
    else
    {
        return ERROR_NOPACKETRECEIVE;
    }
    return SUCCESS;
}

/****************************************************************************
 *  Name:       send_status_packet
 *  Function:   send a status packet
 *  variable:   c_status -- status
 *          n_code   -- error code or status code
 ****************************************************************************/
UINT32 send_status_packet(unsigned char c_status, UINT32 n_code)
{
    MEMSET(&send_packet, 0 , sizeof(PACKET));
    send_packet.packet_type = PACKET_STATUS;
    send_packet.packet_index = g_packet_index;
    send_packet.packet_repeat = g_packet_repeat;
    send_packet.blank_flag = 0;
    send_packet.packet_length = 5;
    send_packet.data_buffer[0] = c_status;
    store_long(send_packet.data_buffer + 1, n_code);
    return packet_send(&send_packet);
}
static UINT8 is_blank_packet(unsigned char *sz_buffer, UINT32 n_len)
{
    UINT8 n_blank_packet = 0;
    UINT8 pkt_base = 2;
    UINT32 i = 0;

    if(!sz_buffer)
    {
        return 1;
    }
    if(0x00 == *sz_buffer)
    {
        n_blank_packet = 2;
    }
    else if(0xFF == *sz_buffer)
    {
        n_blank_packet = 1;
    }
    else
    {
        return 0;
    }
    for(i = 0; i < n_len; i++)
    {
        if(((0x00 != sz_buffer[i]) && (pkt_base == n_blank_packet)) ||
            ((0xFF != sz_buffer[i]) && (1 == n_blank_packet)))
        {
            n_blank_packet = 0;
            break;
        }
    }
    return n_blank_packet;
}
/****************************************************************************
 *  Name:       send_data_packet
 *  Function:   send a data packet
 *  variable:   sz_buffer        -- data buffer to send
 *              n_len            -- data buffer length
 *              n_packet_num      -- packet number about data
 ****************************************************************************/
UINT32 send_data_packet(UINT32 n_packet_num,
                                    unsigned char *sz_buffer, UINT32 n_len)
{
    if(!sz_buffer)
    {
        return !SUCCESS;
    }
    MEMSET(&send_packet, 0, sizeof(PACKET));
    send_packet.blank_flag = is_blank_packet(sz_buffer, n_len);
    send_packet.packet_type = PACKET_DATA;
    send_packet.packet_length = n_len + 4;
    store_long(send_packet.data_buffer, n_packet_num);
    MEMCPY(send_packet.data_buffer + 4, sz_buffer, n_len);
    send_packet.packet_index = g_packet_index;
    send_packet.packet_repeat = g_packet_repeat;

    return packet_send(&send_packet);
}

/****************************************************************************
 *  Name:       send_command_packet
 *  Function:   send a command packet
 *  variable:   sz_command       -- command line string
 *              b_repeat         -- if command repeat
 ****************************************************************************/
UINT32 send_command_packet(char *sz_command, BOOL b_repeat)
{
    if(!sz_command)
    {
        return !SUCCESS;
    }
    MEMSET(&send_packet, 0, sizeof(PACKET));
    send_packet.blank_flag = 0;
    send_packet.packet_type = PACKET_COMMAND;
    send_packet.packet_length = STRLEN(sz_command);
    MEMCPY(send_packet.data_buffer, sz_command, send_packet.packet_length);

    if(b_repeat)
    {
        g_packet_repeat++;
    }
    else
    {
        g_packet_index++;
        g_packet_repeat = 0;
    }
    send_packet.packet_index = g_packet_index;
    send_packet.packet_repeat = g_packet_repeat;

    return packet_send(&send_packet);
}


/****************************************************************************
 *  Name:       send_request_packet
 *  Function:   send a request status packet
 *  variable:   n_packet_num      -- packet number request
 ****************************************************************************/
UINT32 send_request_packet(UINT32 n_packet_num)
{
    MEMSET(&send_packet, 0, sizeof(PACKET));
    send_packet.blank_flag = 0;
    send_packet.packet_type = PACKET_REQUEST;
    send_packet.packet_length = 4;
    store_long(send_packet.data_buffer, n_packet_num);
    send_packet.packet_index = g_packet_index;
    send_packet.packet_repeat = g_packet_repeat;

    return packet_send(&send_packet);
}


