/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2007 Copyright (C)
*
*    File:    net.c
*
*    Description:     This file defines the functions of net device management.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	May.17.2007      Trueve Hu      Ver 0.1    Create file.
*
*****************************************************************************/

#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/net/net.h>

/***********************************************************************
 *
 * 	Name		:	net_open()
 *	Description	:	Open a net device
 *	Parameter	:	struct net_device *dev	: Device to be openned
 *	Return		:	RET_FAILURE			: open device failure 
 *					RET_SUCCESS		: open device success
 *
 ***********************************************************************/
RET_CODE net_open(struct net_device *dev, void (*callback)(struct net_device *, UINT32, UINT32))
{
	RET_CODE result = RET_SUCCESS;
	
	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		PRINTF("net_open: warning - device %s is already openned!\n", dev->name);
		return RET_SUCCESS;
	}
	
	/* Open this device */
	if (dev->open)
	{
		result = dev->open(dev, callback);
	}
	
	/* Setup init work mode */
	if (RET_SUCCESS == result)
	{
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
		
	}
	
	return result;
}

/***********************************************************************
 *
 * 	Name		:	net_close()
 *	Description	:	Close a net device
 *	Parameter	:	struct net_device *dev	: Device to be closed
 *	Return		:	RET_FAILURE 			: close device failure 
 *					RET_SUCCESS		: close device success
 *
 ***********************************************************************/
RET_CODE net_close(struct net_device *dev)
{
	RET_CODE result = RET_SUCCESS;

	if (0 == (dev->flags & HLD_DEV_STATS_UP))
	{
		PRINTF("net_close: warning - device %s is already closed!\n", dev->name);
		return RET_SUCCESS;
	}
	
	/* Stop device */
	if (dev->close)
	{
		result = dev->close(dev);
	}
	
	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	
	return result;
}

/***********************************************************************
 *
 * 	Name		:   net_io_control()
 *	Description	:   Net device IO control command
 *	Parameter	:	struct net_device *dev		: Device
 *					UINT32 cmd				: IO command
 *					UINT32 param				: Param
 *	Return		:	INT32 					: Result
 *
 ***********************************************************************/
RET_CODE net_io_ctrl(struct net_device *dev , UINT32 cmd, UINT32 param)
{
	RET_CODE result = RET_SUCCESS;
	
	if(NULL == dev)
	{
		return ERR_DEV_ERROR;
	}
	/* If device not running, exit */
	/*if (0 == (dev->flags & HLD_DEV_STATS_UP))
	{
		return ERR_DEV_ERROR;
	}*/
	
	if (dev->ioctl)
	{
		result = dev->ioctl(dev, cmd, param);
	}
	else
	{
		result = RET_FAILURE;
	}
	return result;
}

/***********************************************************************
 *
 * 	Name		:	net_send_packet()
 *	Description	:	Net device send packets line
 *	Parameter	:	struct net_device *dev		: Device
 *					void *buf					: packet buffer
 *					UINT16 len				: packet length
 *	Return		:	INT32 					: Result
 *
 ***********************************************************************/
RET_CODE net_send_packet(struct net_device *dev, void *buf, UINT16 len)
{
	RET_CODE result = RET_SUCCESS;
	
	/* If device not running, exit */
	if (0 == (dev->flags & HLD_DEV_STATS_UP))
	{
		return ERR_DEV_ERROR;
	}
	
	if (dev->send_packet)
	{
		if (0 == (dev->flags & NET_DEV_STATS_XMITTING))
		{
			dev->flags |= NET_DEV_STATS_XMITTING;
			result = dev->send_packet(dev, buf, len);
			dev->flags &= ~NET_DEV_STATS_XMITTING;
		}
		else
		{
			result = RET_FAILURE;
		}
	}
	else
	{
		result = RET_FAILURE;
	}
	return result;
}

RET_CODE net_send_packet_ext(struct net_device *dev, struct packseg *packsegs, packseg_free pack_free)
{
	RET_CODE result = RET_SUCCESS;
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}
	
	if (dev->send_packet_ext)
	{
		if ((dev->flags & NET_DEV_STATS_XMITTING) == 0)
		{
			dev->flags |= NET_DEV_STATS_XMITTING;
			result = dev->send_packet_ext(dev, packsegs, pack_free);
			dev->flags &= ~NET_DEV_STATS_XMITTING;
		}
		else
		{
			result = RET_FAILURE;
		}
	}
	else
	{
		result = RET_FAILURE;
	}
	
	return result;
}

/***********************************************************************
 *
 * 	Name		:	net_get_info()
 *	Description	:	Net get device information
 *	Parameter	:	struct net_device *dev		: Device
 *					UINT32 info_type			: information type
 *					void *info_buf				: information buffer
 *	Return		:	INT32 					: Result
 *
 ***********************************************************************/
RET_CODE net_get_info(struct net_device *dev, UINT32 info_type, void *info_buf)
{
	RET_CODE result = RET_SUCCESS;
	
	/* If device not running, exit */
	if (0 == (dev->flags & HLD_DEV_STATS_UP))
	{
		return ERR_DEV_ERROR;
	}
	
	if (dev->get_info)
	{
		result = dev->get_info(dev, info_type, info_buf);
	}
	else
	{
		result = RET_FAILURE;
	}
	return result;
}

/***********************************************************************
 *
 * 	Name		:	net_set_info()
 *	Description	:	Net set device information
 *	Parameter	:	struct net_device *dev		: Device
 *					UINT32 info_type			: information type
 *					void *info_buf				: information buffer
 *	Return		:	INT32 					: Result
 *
 ***********************************************************************/
RET_CODE net_set_info(struct net_device *dev, UINT32 info_type, void *info_buf)
{
	RET_CODE result = RET_SUCCESS;
	
	/* If device not running, exit */
	if (0 == (dev->flags & HLD_DEV_STATS_UP))
	{
		//to set Mac just before open() and after attach(). By FM
		if((NET_SET_MAC != info_type) && (NET_SET_CAPABILITY != info_type))
		{
			return ERR_DEV_ERROR;
		}
	}
	
	if (dev->set_info)
	{
		result = dev->set_info(dev, info_type, info_buf);
	}
	else
	{
		result = RET_FAILURE;
	}
	return result;
}

