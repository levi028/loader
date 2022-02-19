/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: upgrade_command.h
*
*    Description:define variables and functions interface about upgrade commands
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __UPGRADECOMMAND_H__
#define __UPGRADECOMMAND_H__

#include "comm_define.h"

#ifdef __cplusplus
extern "C"
{
#endif
/********************** Macro and Struct Define ******************************/
//define

/********************** extern Define ******************************/

extern unsigned char g_packet_repeat; /* packet_repeat flag */
extern UINT32 g_packet_index;         /* packet index */

/**************************** Function Define *********************************/

/****************************************************************************
 *  Name:       clear_packet_index
 *  Function:   clear global packet index
 ****************************************************************************/

void clear_packet_index(void);
/****************************************************************************
 *  Name:       cmd_version_new
 *  Function:   command version return chunk info
 *  variable:   version     -- get protocol version about STB
 ****************************************************************************/
UINT32 cmd_version_new(UINT32 *version);

/****************************************************************************
 *  Name:       cmd_chunk_new
 *  Function:   command chunk return chunk info
 *  variable:   chunk_info      -- chunk info about command version return
 ****************************************************************************/
UINT32 cmd_chunk_new(void);

/****************************************************************************
 *  Name:       cmd_address_new
 *  Function:   set start address
 *  variable:   addr -- start address
 ****************************************************************************/
UINT32 cmd_address_new(UINT32 addr);

/****************************************************************************
 *  Name:       cmd_move_new
 *  Function:   move chunk
 *  variable:   index -- chunk index
 *              offset - chunk offset
 ****************************************************************************/
UINT32 cmd_move_new(UINT32 index,UINT32 offset);

/****************************************************************************
 *  Name:       cmd_transfer_new
 *  Function:   move chunk
 *  variable:   buffer - data buffer for transfer
 *              n_buf_len -- buffer len
 *              pf_progress      -- callback function about progress
 *              pf_user_abort     -- callback function about if user abort thread
 ****************************************************************************/
UINT32 cmd_transfer_new(unsigned char *buffer, int n_buf_len, CALLBACK_PROGRESS pf_progress,
                                                CALLBACK_USERABORT pf_user_abort);

/****************************************************************************
 *  Name:       cmd_burn_new
 *  Function:   burn flash
 *  variable:   pf_progress      -- callback function about progress
 ****************************************************************************/
UINT32 cmd_burn_new(int n_burn_size, CALLBACK_PROGRESS pf_progress, BOOL b_care_return);

/****************************************************************************
 *  Name:       cmd_burn_new_ext
 *  Function:   burn flash, with user abort
 *  variable:   pf_progress      -- callback function about progress
 ****************************************************************************/
UINT32 cmd_burn_new_ext(int n_burn_size, CALLBACK_PROGRESS pf_progress, BOOL b_care_return,
                                                 CALLBACK_USERABORT pf_user_abort);

/****************************************************************************
 *  Name:       cmd_reboot_new
 *  Function:   reboot target
 ****************************************************************************/
UINT32 cmd_reboot_new();

#ifdef __cplusplus
}
#endif      /*  __cplusplus     */

#endif  /* __UPGRADECOMMAND_H__ */

