/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: dmx_see.h
*
*    Description: This file include the structures, io command and API of
                  demux see module.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef  _DMX_SEE_H_
#define  _DMX_SEE_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include <hld/dmx/dmx_dev_see.h>
#include <hld/dmx/dmx.h>
#define     DMX_SEE_IO                     0x80000000UL

#define     DMX_SEE_GET_AV_SRC_SCRAMBLED  (DMX_SEE_IO+0)
 
RET_CODE dmx_see_io_control(struct dmx_device_see *dev, UINT32 cmd, UINT32 param);

#ifdef __cplusplus
}
#endif

#endif  /*End of _DMX_SEE_H_*/
