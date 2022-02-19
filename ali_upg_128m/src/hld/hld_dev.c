/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: hld_dev.c
*
*    Description:    This file contains all functions definations of
*                     HLD device management.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <bus/otp/otp.h>

static struct hld_device *hld_device_base = NULL;
#ifdef DUAL_ENABLE
#include <modules.h>
static struct remote_hld_device *remote_hld_device_base = NULL;
static UINT8 enable_remote_call = 1;

#define DRAM_SPLIT_CTRL_BASE 0xb8041000
#define PVT_S_ADDR 0x10
void dev_en_remote(UINT8 en)
{
    enable_remote_call = en;
}
#else
void dev_en_remote(UINT8 en)
{
    ;
}

#endif

/*
 *     Name        :   dev_get_by_name()
 *    Description    :   Get a device from device link list by device name.
 *    Parameter    :   INT8 *name                    : Device name
 *    Return        :    void *                        : Device founded
 *
 */
struct hld_device *dev_get_by_name(INT8 *name)
{
    struct hld_device *dev = NULL;
#ifdef DUAL_ENABLE
    struct remote_hld_device *remote_dev = NULL;

#endif
   if(NULL == name)
   {
       ASSERT(0)
       return NULL;
   }
  /* Local device */
    for (dev = hld_device_base; dev != NULL; dev = dev->next)
    {
        /* Find the device */
        if (0 == STRCMP(dev->name, name))
        {
#ifdef SEE_CPU
            /* Only running in the SEE CPU,  need to check if  dev is in private memory in the hld when get device */
            if (!osal_dual_is_private_address(dev))
            {
                return NULL;
            }
#endif
            return dev;
        }
    }

#ifdef DUAL_ENABLE
  /* Remote device */
    if(enable_remote_call)
        {
        if(remote_hld_device_base)
        {
            for (remote_dev = remote_hld_device_base; remote_dev != NULL; remote_dev = remote_dev->next)
            {
                /* Find the device */
                if (0 == STRCMP(remote_dev->name, name))
                {
                    return remote_dev->remote;
                }
            }
        }
        }
#endif
    return NULL;
}

/*
 *     Name        :   dev_get_by_type()
 *    Description    :   Get a device from device link list by device type.
 *    Parameter    :   INT32 type                    : Device type
 *                    void *sdev                    : Start search nod
 *    Return        :    void *                        : Device founded
 *
 */
struct hld_device *dev_get_by_type(void *sdev, UINT32 type)
{
    struct hld_device *dev = NULL;

#ifdef DUAL_ENABLE
    struct remote_hld_device *remote_dev = NULL;

#endif

  /* Local device */
    dev = (NULL == sdev) ? hld_device_base : sdev;

    for (; dev != NULL; dev = dev->next)
    {
        /* Find the device */
        if ((dev->type & HLD_DEV_TYPE_MASK) == type)
        {
#ifdef SEE_CPU
            /* Only running in the SEE CPU,  need to check if  dev is in private memory in the hld when get device */
            if (!osal_dual_is_private_address(dev))
            {
                return NULL;
            }
#endif
            return dev;
        }
    }

#ifdef DUAL_ENABLE
  /* Remote device */
 if(enable_remote_call)
 {
        if(remote_hld_device_base)
        {
            for (remote_dev = remote_hld_device_base; remote_dev != NULL; remote_dev = remote_dev->next)
            {
                /* Find the device */
                if ((remote_dev->type & HLD_DEV_TYPE_MASK) == type)
                {
                    return remote_dev->remote;
                }
            }
        }
 }
#endif
    return NULL;
}

/*
 *     Name        :   dev_get_by_id()
 *    Description    :   Get a device from device link list by device ID.
 *    Parameter    :   UINT32 type                    : Device type
 *                    UINT16 id                    : Device id
 *    Return        :    void *                        : Device founded
 *
 */
struct hld_device *dev_get_by_id(UINT32 type, UINT16 id)
{
    struct hld_device *dev = NULL;

#ifdef DUAL_ENABLE
    struct remote_hld_device *remote_dev = NULL;

#endif

  /* Local device */
    for (dev = hld_device_base; dev != NULL; dev = dev->next)
    {
        /* Find the device */
        if (dev->type == (type | id))
        {
#ifdef SEE_CPU
            /* Only running in the SEE CPU,  need to check if  dev is in private memory in the hld when get device */
            if (!osal_dual_is_private_address(dev))
            {
                return NULL;
            }
#endif        
            return dev;
        }
    }
#ifdef DUAL_ENABLE
    /* Remote device */
 if(enable_remote_call)
 {
        if(remote_hld_device_base)
        {
            for (remote_dev = remote_hld_device_base; remote_dev != NULL; remote_dev = remote_dev->next)
            {
                if (remote_dev->type == (type | id))
                {
                    return remote_dev->remote;
                }
            }
      }
 }
#endif
    return NULL;
}

/*
 *     Name        :   hld_dev_add()
 *    Description    :   Add a device to device link list tail.
 *    Parameter    :    struct hld_device *dev        : Device need queue to tail
 *    Return        :    INT32                        : Result value
 *
 */
static INT32 hld_dev_add(struct hld_device *dev)
{
    struct hld_device *dp = NULL;
    UINT32 count = 0;
    BOOL ret = RET_FAILURE;

    /* Is a null queue */
    if (NULL == hld_device_base)
    {
        hld_device_base = dev;
        dev->next = NULL;
#ifdef  DUAL_ENABLE
        if(enable_remote_call)
        {
            ret = hld_dev_add_remote((struct hld_device *)dev, (UINT32)(dev));
			if(ret != SUCCESS)
			{
				return RET_FAILURE;
			}
        }
#endif
        return SUCCESS;
    }

    if (dev_get_by_name(dev->name) != NULL)
    {
        PRINTF("hld_dev_add error: device %s same name!\n", dev->name);
        return ERR_DEV_CLASH;
    }

    /* Add this device to device list */
    /* Move to tail */
    for (dp = hld_device_base, count = 0; dp->next != NULL; dp = dp->next)
    {
        count++;
    }

    if ((count + 1) > HLD_MAX_DEV_NUMBER)
    {
        PRINTF("\n hld_dev_add error: device number beyond the maximum value =  %d !\n", HLD_MAX_DEV_NUMBER);
        return ERR_QUEUE_FULL;
    }

    dp->next = dev;
    dev->next = NULL;
#ifdef  DUAL_ENABLE
    if(enable_remote_call)
    {
        ret = hld_dev_add_remote((struct hld_device *)dev, (UINT32)(dev));
	    if(ret != SUCCESS)
	    {
		    return RET_FAILURE;
	    }
    }
#endif

    return SUCCESS;
}

/*
 *     Name        :   hld_dev_remove()
 *    Description    :   Remove a device from device link list.
 *    Parameter    :    struct hld_device *dev        : Device to be remove
 *    Return        :    INT32                        : Result value
 *
 */
static INT32 hld_dev_remove(struct hld_device *dev)
{
    struct hld_device *dp = NULL;
    BOOL ret = RET_FAILURE;

    struct hld_device *dp_tmp = NULL;

    /* If dev in dev_queue, delete it from queue, else free it directly */
    if (hld_device_base != NULL)
    {
        if (0 == STRCMP(hld_device_base->name, dev->name))
        {
             /* Store the hld_device_base in the temperary variable before being modified */
            dp_tmp = hld_device_base;
            hld_device_base = dev->next;

            if (dp_tmp != dev)
            {
                hld_device_base = dp_tmp->next;
                
                /* If the input parameter : dev  is  not the pointer to the node of the linked list,  free the memory of this node, 
                     otherwize   the memory of this node will be freed in the interface: void dev_free(void *dev) */                  
                osal_memory_free(dp_tmp);
            }
            
#ifdef  DUAL_ENABLE
        if(enable_remote_call)
        {
            ret = hld_dev_remove_remote((struct hld_device *)dev);
			if(ret != SUCCESS)
		    {
			    return RET_FAILURE;
		    }
        }
#endif
        }
        else
        {
            for (dp = hld_device_base; dp->next != NULL; dp = dp->next)
            {
                if (0 == STRCMP(dp->next->name, dev->name))
                {
                    /* Store the dp->next in the temperary variable before being modified */
                    dp_tmp = dp->next;
                    
                    dp->next = dev->next;

                     if (dp_tmp != dev)
                    {
                        dp->next = dp_tmp->next;    
                        
                        /* If the input parameter : dev  is  not the pointer to  the node of the linked list,  free the memory of this node, 
                             otherwize   the memory of this node will be freed in the interface: void dev_free(void *dev) */                  
                        osal_memory_free(dp_tmp);
                    }
                     
#ifdef  DUAL_ENABLE
                    if(enable_remote_call)
                    {
                        ret = hld_dev_remove_remote((struct hld_device *)dev);
						if(ret != SUCCESS)
					    {
						    return RET_FAILURE;
					    }
                    }
#endif
                    break;
                }
            }
        }
    }

    return SUCCESS;
}

/*
 *     Name        :   dev_alloc()
 *    Description    :   Alloc and add a HLD device to device link list.
 *    Parameter    :   INT8 *name                    : Device's name
 *                    UINT32 size                    : Device structure size in bytes
 *    Return        :    void*                         : Device need be added
 *
 */

struct hld_device *dev_alloc(INT8 *name, UINT32 type, UINT32 size)
{
    struct hld_device *dev = NULL;//(struct hld_device *)osal_memory_allocate(size);
    struct hld_device *dp = NULL;
    UINT16 id = 0;
    UINT8 str_len = 0;

    if(size<sizeof(struct hld_device))
    {
        return NULL;
    }

    dev = (struct hld_device *)osal_memory_allocate(size);

    if (NULL == dev)
    {
        PRINTF("hld_dev_alloc: error - device %s not enough memory: %08x!\n",
              name, size);
        return NULL;
    }

    if(NULL == name)
    {
        ASSERT(0);
        osal_memory_free(dev);
        return NULL ;
    }
    /* Name same with some one in HLD device list, error */
    for (id = 0, dp = hld_device_base; dp != NULL; dp = dp->next)
    {
        /* Find the device */
        if (0 == STRCMP(dp->name, name))
        {
            PRINTF("hld_dev_alloc: error - device %s same name!\n", name);
            osal_memory_free(dev);
            return NULL;
        }
        /* Check ID */
        if ((dp->type & HLD_DEV_TYPE_MASK) == type)
        {
            id++;
        }
    }

    /* Clear device structure */
    MEMSET((UINT8 *)dev, 0, size);
    dev->type = (type | id);

    str_len = strlen(name);
    if(HLD_MAX_NAME_SIZE < str_len)
    {
        str_len = HLD_MAX_NAME_SIZE;
    }
    strncpy(dev->name, name, str_len);

    return dev;
}

/*
 *     Name        :   dev_register()
 *    Description    :   Register a HLD device to system.
 *    Parameter    :    void *dev                    : Device need be register
 *    Return        :   INT8                        : Return value
 *
 */
INT32 dev_register(void *dev)
{
    return hld_dev_add((struct hld_device *)dev);
}

/*
 *     Name        :   dev_free()
 *    Description    :   Free a HLD device from device link list.
 *    Parameter    :    void *dev                    : Device to be free
 *    Return        :
 *
 */
void dev_free(void *dev)
{
    if (dev != NULL)
    {
        hld_dev_remove((struct hld_device *)dev);
        osal_memory_free(dev);
    }

    return;
}

/*
 *     Name        :   dev_list_all(void *sdev)
 *    Description    :   Print out all device from device link list.
 *    Parameter    :    void *sdev                    : Start search nod
 *    Return        :
 *
 */
void dev_list_all(void *sdev)
{
    struct hld_device *dev = NULL;

    dev = (NULL == sdev) ? hld_device_base : sdev;

    for (dev = hld_device_base; dev != NULL; dev = dev->next)
    {
        PRINTF("Name: %16s; Type: 0x%08x; Address: 0x%08x\n",
          dev->name, dev->type, dev);
    }

    return;
}

#ifdef  DUAL_ENABLE

INT32 remote_hld_dev_add(struct hld_device *buf, UINT32 dev_addr)
{
    UINT32 count = 0;
    struct remote_hld_device *dev = NULL;
    struct remote_hld_device *dp = NULL;

    dev = (struct remote_hld_device *)MALLOC(sizeof(struct remote_hld_device));
    if (NULL == dev)
    {
        return !SUCCESS;
    }
    if(NULL == buf)
    {
        ASSERT(0);
        return !SUCCESS;
    }
    MEMCPY(dev, buf, sizeof(struct hld_device));
    dev->remote = (struct hld_device *)(dev_addr);
    dev->next = NULL;
    if(NULL == remote_hld_device_base)
    {
        remote_hld_device_base = dev;
    }
    else
    {
        if (dev_get_by_name(dev->name) != NULL)
        {
            PRINTF("hld_dev_add error: device %s same name!\n", dev->name);
            ASSERT(0);
        }

        /* Add this device to device list */
        /* Move to tail */
        for (dp = remote_hld_device_base, count = 0; dp->next != NULL; dp = dp->next)
        {
            count++;
        }
          
        if ((count + 1) > HLD_MAX_DEV_NUMBER)
        {
            PRINTF("\n remote_hld_dev_add error: device number  beyond the maximum value =  %d !\n", HLD_MAX_DEV_NUMBER);
            return ERR_QUEUE_FULL;
        }

        dp->next = dev;
    }

/*    if(remote_hld_device_base)
    {
        dp = remote_hld_device_base;
        do
        {
            libc_printf("dev name: %s, type: 0x%x, dev: 0x%x, remote: 0x%x\n", dp->name, dp->type, dp, dp->remote);
            dp = dp->next;
        }while (dp != NULL);
    }*/
    return SUCCESS;
}

INT32 remote_hld_dev_remove(struct hld_device *dev)
{
       struct remote_hld_device *dp = NULL;
       struct remote_hld_device *dp_tmp = NULL;

    if(NULL == dev)
    {
        ASSERT(0);
        return !SUCCESS;
    }
    /* If dev in dev_queue, delete it from queue, else free it directly */
    if (remote_hld_device_base != NULL)
    {
        if (0 == STRCMP(remote_hld_device_base->name, dev->name))
        {
            /* Store the remote_hld_device_base in the temperary variable before being modified */
            dp_tmp = remote_hld_device_base;
            remote_hld_device_base = remote_hld_device_base->next;

            /* free the memory of this node */
            FREE(dp_tmp);
        }
        else
        {
            for (dp = remote_hld_device_base; dp->next != NULL; dp = dp->next)
            {
                if (0 == STRCMP(dp->next->name, dev->name))
                {
                    /* Store the dp->next in the temperary variable before being modified */
                    dp_tmp = dp->next;
                    dp->next = dp->next->next;

                    /* free the memory of this node */
                    FREE(dp_tmp);                    
                    break;
                }
            }
        }
    }

    return SUCCESS;

}

void remote_hld_dev_memcpy(__attribute__((unused))void *dest, __attribute__((unused))const void *src, __attribute__((unused))unsigned int len)
{
    #if 0
    void *ret = NULL;

    if(NULL != dest && NULL != src)
    {
        osal_cache_invalidate((void *)src, len);
        ret = ali_memcpy(dest, src,len);
        if (0x80000000 == (((UINT32)dest)&0x80000000))
        {
            osal_cache_flush(dest, len);
        }
    }
  #endif
}

void remote_cpy_from_priv_mem(__attribute__((unused))void *dest,__attribute__((unused))UINT32 data_len)
{
    return;
}

void remote_hld_see_init(void *addr)
{
#if (defined(_M3715C_) || defined(_M3823C_) || defined(_STATIC_SEE_MEMORY_MAP_))
//#if (defined(_M3715C_) || defined(_M3823C_))


    return;
#else   
    if(NULL != addr)
    {
        see_heaptop_init((unsigned int)addr);
    }
#endif
}

void remote_otp_get_mutex(UINT32 flag)
{
#ifndef _BL_ENABLE_
    ali_otp_get_mutex(flag);
#endif
}

void remote_enable_hd_decoder(UINT32 flag)
{
    sys_ic_set_hd_decoder(flag);
}

#endif

