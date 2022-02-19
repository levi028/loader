/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: packet.h
*
*    Description: define information packet variable & function for commicate
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __PACKET_H__
#define __PACKET_H__

#ifdef __cplusplus
extern "C"
{
#endif

/********************************* Variable Define ****************************/

#define PACKET_HEAD_LEN     19
#define MAX_PACKET_SIZE     2048
#define STATUS_DATA_LEN     5
#define PACKET_HEAD_FLAG_OFFSET     0
#define PACKET_TYPE_OFFSET          4
#define PACKET_BLANKFLAG_OFFSET     5
#define PACKET_INDEX_OFFSET         6
#define PACKET_REPEAT_OFFSET        10
#define PACKET_LENGTH_OFFSET        11
#define PACKET_HEAD_CRC_OFFSET      15
#define sleep(a)        osal_delay(a * 1000)

typedef enum
{
    PACKET_COMMAND = 0,
    PACKET_DATA,
    PACKET_REQUEST,
    PACKET_STATUS
}PACKET_TYPE;

typedef enum
{
    COMMAND_STATUS_OK = 0,
    COMMAND_STATUS_ERROR,
    COMMAND_STATUS_RUNNING,
    COMMAND_STATUS_EXECUTED
}COMMAND_STATUS;

typedef struct
{
    unsigned char   packet_type;
    unsigned char   blank_flag;
    UINT32          packet_index;
    unsigned char   packet_repeat;
    UINT32          packet_length;
    unsigned char   data_buffer[MAX_PACKET_SIZE];
}PACKET, * PPACKET;

/********************************* Function Define *****************/
void com_clear_buffer(void);
UINT32 com_read_tm(unsigned char *buffer, UINT32 n_len, UINT32 n_time_out);
UINT32 com_read(unsigned char *buffer, UINT32 n_len);
UINT32 com_send(unsigned char * buffer, UINT32 n_len);
UINT32 com_send_char(unsigned char c);
UINT32 com_send2back_tm(unsigned char * buffer, UINT32 n_len, UINT32 n_time_out);

/****************************************************************************
 *  Name:       packet_send
 *  Function:   send a packet
 *  variable:   p_packet     -- packet information point
 ****************************************************************************/
UINT32 packet_send(PPACKET p_packet);

/****************************************************************************
 *  Name:       packet_receive
 *  Function:   receive a packet
 *  variable:   p_packet     -- packet information point
 *              n_time_out    -- time out
 ****************************************************************************/
UINT32 packet_receive(PPACKET p_packet, UINT32 n_time_out);


/****************************************************************************
 *  Name:       send_status_packet
 *  Function:   send a status packet
 *  variable:   c_status -- status
 *          n_code   -- error code or status code
 ****************************************************************************/
UINT32 send_status_packet(unsigned char c_status, UINT32 n_code);

/****************************************************************************
 *  Name:       send_data_packet
 *  Function:   Send a data Packet
 *  variable:   sz_buffer        -- data buffer to send
 *              n_len            -- data buffer length
 *              n_packet_num      -- packet number about data
 ****************************************************************************/
UINT32 send_data_packet(UINT32 n_packet_num, unsigned char *sz_buffer, UINT32 n_len);

/****************************************************************************
 *  Name:       send_command_packet
 *  Function:   Send a command Packet
 *  variable:   sz_command       -- command line string
 *              b_repeat         -- if command repeat
 ****************************************************************************/
UINT32 send_command_packet(char * sz_command, BOOL b_repeat);

/****************************************************************************
 *  Name:       send_request_packet
 *  Function:   send a request status packet
 *  variable:   n_packet_num      -- packet number request
 ****************************************************************************/
UINT32 send_request_packet(UINT32 n_packet_num);

#ifdef __cplusplus
}
#endif      /*  __cplusplus     */

#endif  /* __PACKET_H__ */

