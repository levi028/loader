/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: dmx_see.c
*
*    Description: This file includes the realization of DMX SEE API for SEE module.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <hld/dmx/dmx_see.h>
#include <hld/dmx/dmx_dev_see.h>

RET_CODE dmx_see_io_control
(
    struct dmx_device_see *dev,
    UINT32                 cmd,
    UINT32                 param
)
{
	RET_CODE ret = RET_FAILURE;

	
	if(NULL == dev)
	{
		return ret;
	}
		
	if(dev->ioctl)
	{
	    ret = dev->ioctl(dev, cmd, param);
	}

	return ret;
}

