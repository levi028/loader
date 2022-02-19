/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: upgrade_commond_old.h
*    Description:define variables and functions interface about upgrade module
*                     use old protocol version
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __UPGRADECOMMAND_OLD_H__
#define __UPGRADECOMMAND_OLD_H__

#include "comm_define.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Variable Define ******************************/

/***************************** Function Define ******************************/
/****************************************************************************
 *  Name:       cmd_version_old
 *  Function:   command version return chunk info
 *  variable:   p_version        -- protocol version (return value)
 ****************************************************************************/
unsigned int cmd_version_old(UINT32 *p_version);

/****************************************************************************
 *  Name:       cmd_address_old
 *  Function:   set start address
 *  variable:   addr -- start address
 ****************************************************************************/
unsigned int cmd_address_old(unsigned int addr);
/****************************************************************************
 *  Name:       cmd_move_old
 *  Function:   move chunk
 *  variable:   index -- chunk index
 *              offset - chunk offset
 ****************************************************************************/
unsigned int cmd_move_old(unsigned int index,unsigned int offset);

/****************************************************************************
 *  Name:       cmd_transferraw_old
 *  Function:   move chunk
 *  variable:   buffer - data buffer for transfer
 *              n_buf_len -- buffer len
 *              pf_progress      -- callback function about progress
 *              pf_user_abort     -- callback function about if user abort thread
 ****************************************************************************/
unsigned int cmd_transferraw_old(unsigned char *buffer, int n_buf_len, CALLBACK_PROGRESS pf_progress,
                                                            CALLBACK_USERABORT pf_user_abort);

/****************************************************************************
 *  Name:       cmd_burn_old
 *  Function:   burn flash
 *  variable:   pf_progress      -- callback function about progress
 ****************************************************************************/
unsigned int cmd_burn_old(int n_burn_size,  CALLBACK_PROGRESS pf_progress, BOOL b_care_return);

/****************************************************************************
 *  Name:       cmd_reboot_old
 *  Function:   reboot target
 ****************************************************************************/
unsigned int cmd_reboot_old();

#ifdef __cplusplus
}
#endif      /*  __cplusplus     */

#endif  /* __UPGRADECOMMAND_OLD_H__ */

