/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sie_eit.h
*
*    Description: process EIT table
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __SIE_EIT_H__
#define __SIE_EIT_H__

#ifdef __cplusplus
extern "C" {
#endif

INT32 stop_eit_ext(void);
INT32 start_eit_ext(struct dmx_device *dmx, struct sie_eit_config *config,
    si_dmx_event_t event, si_handler_t handle,UINT8 mode);

#ifdef __cplusplus
}
#endif

#endif
