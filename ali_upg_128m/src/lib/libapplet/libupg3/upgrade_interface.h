/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: upgrade_interface.h
*    Description:define variables and functions interface about upgrade module
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __UPGRADEINTERFACE_H__
#define __UPGRADEINTERFACE_H__

#include "comm_define.h"

#ifdef __cplusplus
extern "C"
{
#endif
/********************* Macro and Struct Define *****************************/

/**************************** Function Define *********************************/
/****************************************************************************
 *  Name:       set_protocol_version()
 *  Function:   set client tools(Boot Loader) protocol version
 ****************************************************************************/
void set_protocol_version(UINT32 n_protocol_version);

/****************************************************************************
 *  Name:       cmd_comtest
 *  Function:   test serial port communication
 *  variable:   pf_progress      -- callback function about progress
 *              pf_user_abort     -- callback function about if user abort thread
 ****************************************************************************/
UINT32 cmd_comtest(UINT32 *p_version, CALLBACK_PROGRESS pf_progress,  CALLBACK_USERABORT pf_user_abort);


/****************************************************************************
 *  Name:       cmd_version
 *  Function:   command version return chunk info
 *  variable:   p_version        -- protocol version (return value)
 ****************************************************************************/
UINT32 cmd_version(UINT32 *p_version);


/****************************************************************************
 *  Name:       cmd_address
 *  Function:   set start address
 *  variable:   addr -- start address
 ****************************************************************************/
UINT32 cmd_address(UINT32 addr);

/****************************************************************************
 *  Name:       cmd_move
 *  Function:   move chunk
 *  variable:   index -- chunk index
 *              offset - chunk offset
 ****************************************************************************/
UINT32 cmd_move(UINT32 index,UINT32 offset);

/****************************************************************************
 *  Name:       cmd_transfer
 *  Function:   move chunk
 *  variable:   buffer - data buffer for transfer
 *              n_buf_len -- buffer len
 *              pf_progress      -- callback function about progress
 *              pf_user_abort     -- callback function about if user abort thread
 ****************************************************************************/
UINT32 cmd_transfer(unsigned char *buffer, int n_buf_len, CALLBACK_PROGRESS pf_progress,  CALLBACK_USERABORT pf_user_abort);

/****************************************************************************
 *  Name:       cmd_burn
 *  Function:   burn flash
 *  variable:   pf_progress      -- callback function about progress
 ****************************************************************************/
UINT32 cmd_burn(int n_burn_size, CALLBACK_PROGRESS pf_progress, BOOL b_care_return);

/****************************************************************************
 *  Name:       cmd_burn_ext
 *  Function:   burn flash, with user abort
 *  variable:   pf_progress      -- callback function about progress
 ****************************************************************************/
UINT32 cmd_burn_ext(int n_burn_size, CALLBACK_PROGRESS pf_progress,  BOOL b_care_return,  CALLBACK_USERABORT pf_user_abort);

/****************************************************************************
 *  Name:       cmd_reboot
 *  Function:   reboot target
 ****************************************************************************/
UINT32 cmd_reboot();
#ifdef __cplusplus
}
#endif      /*  __cplusplus     */

#endif  /* __UPGRADEINTERFACE_H__ */

