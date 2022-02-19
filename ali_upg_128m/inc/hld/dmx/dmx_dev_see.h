/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: dmx_dev_see.h
*
*    Description: This file include the structures of demux see device.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _DMX_DEV_SEE_H_
#define _DMX_DEV_SEE_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include <osal/osal.h>
#include <sys_config.h>
#include <mediatypes.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>

struct dmx_device_see
{
	struct dmx_device_see *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[HLD_MAX_NAME_SIZE];
	INT32 flags;

	INT32 hardware;
	INT32 busy;
	INT32 minor;

	void *priv;		/* Used to be 'private' but that upsets C++ */
	UINT32 base_addr;
	
	RET_CODE (*ioctl)(struct dmx_device_see *dev, UINT32 cmd, UINT32 param);	
};
#ifdef __cplusplus
 }
#endif

#endif /* _DMX_DEV_SEE_H_ */


