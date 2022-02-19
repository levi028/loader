/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: sto_flash.c
*
*    Description: Provide general flash driver for sto type device.
*
*    History:
*      Date        Author      Version  Comment
*      ====        ======      =======  =======
*  0.              Justin Wu            Ref. code
*  1.  2005.5.28   Liu Lan     0.1.000  Initial
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <sys_parameters.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_mem.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <bus/flash/flash.h>
#include <bus/erom/erom.h>
#include <bus/erom/uart.h>
#include <asm/chip.h>
#include "sto_flash.h"
#include "flash_data.h"

#if (defined(UNPROTECTED_DATABASE_ADDR_LOW) && defined(UNPROTECTED_DATABASE_ADDR_UP))
BOOL m_enable_soft_protection = TRUE;
#else
BOOL m_enable_soft_protection = FALSE;
#endif
#ifdef UNPROTECTED_DATABASE_ADDR_LOW
unsigned long unpro_addr_low = UNPROTECTED_DATABASE_ADDR_LOW;
#else
unsigned long unpro_addr_low = SYS_FLASH_BASE_ADDR;
#endif
#ifdef UNPROTECTED_DATABASE_ADDR_UP
unsigned long unpro_addr_up = UNPROTECTED_DATABASE_ADDR_UP;
#else
unsigned long unpro_addr_up = SYS_FLASH_BASE_ADDR + 0x400000;
#endif
unsigned long remote_flash_read_tmo = RF_READ_TMO;

//#define FLASH_ADDR_MASK     0xFFFFFF

static OSAL_ID sto_mutex_id_ex = OSAL_INVALID_ID;
enum
{
    FREE ,
    BUSY ,
    SUSPEND,
    RESUME ,
};
/*mutex for software flash device status*/
static UINT32 ref_status = 0;
static void sto_mutex_enter_ex(void)
{
#if ( SYS_OS_MODULE  == ALI_TDS2 )
    if(OSAL_INVALID_ID==sto_mutex_id_ex)
    {
        sto_mutex_id_ex = osal_mutex_create();
        ref_status = FREE ;
        ASSERT(sto_mutex_id_ex!=OSAL_INVALID_ID);
    }
    osal_mutex_lock(sto_mutex_id_ex, OSAL_WAIT_FOREVER_TIME);
#endif
    ref_status = BUSY ;
}
static void sto_mutex_exit_ex(void)
{
 #if (SYS_OS_MODULE == ALI_TDS2 )
    osal_mutex_unlock(sto_mutex_id_ex);
 #endif
    ref_status = FREE ;
}

#ifdef STO_PROTECT_BY_MUTEX
/*mutex for hardware controller/bus source */
static OSAL_ID sto_mutex_id = OSAL_INVALID_ID;
#endif
void sto_mutex_enter(void)
{
#ifdef STO_PROTECT_BY_MUTEX
    if(OSAL_INVALID_ID==sto_mutex_id)
    {
        sto_mutex_id = osal_mutex_create();
        ASSERT(sto_mutex_id!=OSAL_INVALID_ID);
    }
    osal_mutex_lock(sto_mutex_id, OSAL_WAIT_FOREVER_TIME);
#endif
}

void sto_mutex_exit(void)
{
#ifdef STO_PROTECT_BY_MUTEX
    osal_mutex_unlock(sto_mutex_id);
#endif
}

static INT32 sto_flash_open(struct sto_device *dev)
{
    int id = 0;
    struct flash_private *tp = NULL;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct flash_private *)dev->priv;
    if (OSAL_INVALID_ID == (id = osal_task_get_current_id()))
    {
        id = 0;
    }
    if(id>=STO_TASK_SUPPORT_NUM_MAX)
    {
        //Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
        ASSERT(0);
        return ERR_FAILED;
    }
    dev->curr_addr[id] = 0;

    if(tp->open)
    {
        return tp->open((void *)dev);
    }

    return SUCCESS;
}

static INT32 sto_flash_close(struct sto_device *dev)
{
    struct flash_private *tp = NULL;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct flash_private *)dev->priv;
    if(tp->close)
    {
        return tp->close((void *)dev);
    }

     return SUCCESS;
}

static INT32 sto_flash_lseek(struct sto_device *dev, INT32 offset, \
      int origin)
{
    INT32 new_addr = 0;
    int id = 0;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    if (OSAL_INVALID_ID == (id = osal_task_get_current_id()))
    {
        id = 0;
    }
    if(id>=STO_TASK_SUPPORT_NUM_MAX)
    {
        //Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
        ASSERT(0);
        return ERR_FAILED;
    }
    switch (origin)
    {
        case STO_LSEEK_SET:
            /* Great than totol size, seek to end */
            if (offset >= dev->totol_size)
            {
                dev->curr_addr[id] = dev->totol_size - 1;
            }
            /* Common seek */
            else if (offset >= 0)
            {
                dev->curr_addr[id] = offset;
            }
            break;

        case STO_LSEEK_CUR:
            new_addr = dev->curr_addr[id] + offset;
            /* Less than base address, seek to begin */
            if (new_addr < 0)
            {
                dev->curr_addr[id] = 0;
            }
            /* Great than totol size, seek to end */
            else if (new_addr >= dev->totol_size)
            {
                dev->curr_addr[id] = dev->totol_size - 1;
            }
            /* Common seek */
            else
            {
                dev->curr_addr[id] = new_addr;
            }
            break;

        case STO_LSEEK_END:
            new_addr = dev->totol_size + offset - 1;
            /* Less than base address, seek to begin */
            if (new_addr < 0)
            {
                dev->curr_addr[id] = 0;
            }
            /* Common seek */
            else if (offset <= 0)
            {
                dev->curr_addr[id] = new_addr;
            }
            break;
        default:
            break;
    }

    return dev->curr_addr[id];
}


/*
 * Auto erase sectors according start offset and length.
 */
static INT32 sto_flash_auto_sec_erase(struct sto_device *dev, \
              UINT32 target_start, INT32 len_in_k)
{
    unsigned char *p = NULL;
    unsigned char *oper_p = NULL;
    unsigned long oper_start = 0;
    unsigned long oper_end = 0;
    unsigned long oper_len = 0;
    unsigned long l = 0;
    unsigned long target_end = 0;
    unsigned int sector_no = 0;
    unsigned long sector_start = 0;
    unsigned long sector_end = 0;
    unsigned long sector_size = 0;
    unsigned int ret = 0;
    int rcode = 0;
    struct flash_private *tp = (struct flash_private *)dev->priv;

    /* check input data validation */
    if (target_start >= tp->flash_size)
    {
        return ERR_FLASH_OFFSET_ERROR;
    }
    /* limit input data to proper value */
    target_end = target_start + (len_in_k << 10);
    if (target_end > tp->flash_size)
    {
        target_end = tp->flash_size;
    }
    /* begin to erase sector by sector */
    sector_no = sto_flash_sector_align(dev,target_start);
    sector_start = sto_flash_sector_start(dev,sector_no);
    while (target_start < target_end)
    {
        /* get information about current sector */
        sector_size = sto_flash_sector_size(dev,sector_no);
        sector_end = sector_start + sector_size;
        /* get information about current operation */
        oper_start = target_start > sector_start ? target_start : \
              sector_start;
        oper_end = target_end < sector_end ? target_end : \
              sector_end;
        oper_len = oper_end - oper_start;
        oper_p = NULL;
        /* check if need keep data */
        if (oper_len != sector_size)
        {
            oper_p = dev->sector_buffer;
            p = dev->sector_buffer;
            /* copy pre-fix block from flash */
            l = oper_start - sector_start;
            if (l != 0)
            {
                /* check if combination buffer existed */
                if (NULL == oper_p)
                {
                    return ERR_FLASH_NO_BUFFER;
                }
                if(0 == remote_flash_read_tmo)
                {
                    tp->read(p, FLASH_OFF2PTR(sector_start), l);
                }
                else
                {
                    if(SUCCESS!=(tp->read(p, FLASH_OFF2PTR(sector_start),l)))
                    {
                        return ERR_FLASH_READ_FAIL;
                    }
                }
                p += l;
            }
            if (oper_p != NULL)
            {
                /* set erase area */
                MEMSET(p, 0xFF, oper_len);
                p += oper_len;
            }
            if (dev->flag & STO_FLAG_SAVE_REST)
            {
                /* copy post-fix block from flash */
                l = sector_end - oper_end;
                if (l != 0)
                {
                    /* check if combination buffer existed */
                    if (NULL == oper_p)
                    {
                        return ERR_FLASH_NO_BUFFER;
                    }
                    if(0 == remote_flash_read_tmo)
                    {
                        tp->read(p, FLASH_OFF2PTR(oper_end), l);
                    }
                    else
                    {
                        if(SUCCESS!=(tp->read(p, FLASH_OFF2PTR(oper_end), l)))
                        {
                            return ERR_FLASH_READ_FAIL;
                        }
                    }
                    p += l;
                }
            }
            /* update operation information */
            if (oper_p != NULL)
            {
                oper_len = p - oper_p;
            }
            oper_start = sector_start;
        }
        //MUTEX_ENTER();
        /* erase sector */
        rcode = tp->erase_sector(sector_start);
        //MUTEX_LEAVE();
        if(0 == remote_flash_read_tmo)
        {
            if(SUCCESS!=rcode)
            {
                return ERR_FLASH_NEED_ERASE;
            }
        }/*REMOTE_FLASH_READ_TMO */
        /* check if need write back */
        if (oper_p != NULL)
        {
            //MUTEX_ENTER();
            ret = tp->write(oper_start, oper_p, oper_len);
            //MUTEX_LEAVE();
            if (ret != 0)
            {
                return ERR_FLASH_WRITE_FAIL;
            }
        }
        /* advance to next data block (sector) */
        target_start += oper_len;
        sector_start = sector_end;
        sector_no++;
    }

    return SUCCESS;
}

/**************************************************************
 * Function:
 *     sto_sflash_info()
 * Description:
 *     provide upper the information of this flash.
 * Inpute:
 *    *flash_size
 *    *sector_size
 *    *total_sector
 * Return Values:
 *  None
 ***************************************************************/
static void sto_sflash_info(UINT32 param)
{
    struct sto_device *sflash_dev = NULL;
    struct flash_private *tp = NULL;
    struct flash_info *p_flash_info = NULL;

    p_flash_info = (struct flash_info *)param;

    sflash_dev = (struct sto_device *)dev_get_by_name("STO_SFLASH_0");
    if(sflash_dev == NULL)
        return;
    tp = (struct flash_private *)sflash_dev->priv;

    p_flash_info->flash_size = tp->flash_size;
    p_flash_info->total_sectors = tp->flash_sectors;
    p_flash_info->sector_size = sto_flash_sector_size(sflash_dev,tp->flash_sectors-1);
}

/***************************************************************
 *     Name        :   sto_flash_ioctl()
 *    Description    :   Do miscellaneous routines.
 *    Parameter    :    INT32 cmd, UINT32 param
 *    Return        :    INT32
 *  Current support:
 *     .Chip erase;
 *     .Sector auto erase;
 */
static INT32 sto_flash_ioctl(struct sto_device *dev, INT32 cmd, \
      UINT32 param)
{
    UINT32 start_offset = 0;
    INT32 len_in_k = 0;
    INT32 rcode = 0;
    struct flash_private *tp = NULL;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    tp = (struct flash_private *)dev->priv;
    /*common IO_Control of sto device*/
    switch (cmd)
    {
#if 0
        case STO_DRIVER_ATTACH:
        case STO_DRIVER_SUSPEND:
        case STO_DRIVER_RESUME:
        case STO_DRIVER_DETACH:
        case STO_DRIVER_GET_STATUS:
        case STO_DRIVER_SELECT_SECTOR:
        case STO_DRIVER_DEVICE_PROTE:
        case STO_DRIVER_SECTOR_PROTE:
            return SUCCESS;
#endif
        case STO_DRIVER_SUSPEND:
            sto_mutex_enter_ex();
            ref_status = SUSPEND ;
            return SUCCESS;

        case STO_DRIVER_RESUME:
            sto_mutex_exit_ex();
            ref_status = RESUME ;
            return SUCCESS;
        case STO_DRIVER_DEVICE_ERASE:
            sto_mutex_enter_ex();
            rcode=tp->erase_chip();
            sto_mutex_exit_ex();
            if(0 == remote_flash_read_tmo)
            {
                return SUCCESS;
            }
            break;
            /*
            else
            {
                if(SUCCESS!=rcode)
                {
                    return ERR_FLASH_NEED_ERASE;
                }
                else
                {
                    return SUCCESS;
                }
            }
            */
        case STO_DRIVER_SECTOR_ERASE: /* Auto erase sectors */
            /* Uper 22 bits of MSB is start offset */
            start_offset = (param >> 10);
            /* Lower 10 bits of LSB is length in K bytes*/
            len_in_k = (param & 0x3ff);
            sto_mutex_enter_ex();
            rcode= sto_flash_auto_sec_erase(dev, start_offset, \
              len_in_k);
            sto_mutex_exit_ex();
            return rcode;
        case STO_DRIVER_SECTOR_ERASE_EXT: /* Auto erase sectors */
            start_offset = ((UINT32 *)param)[0];
            len_in_k = ((UINT32 *)param)[1];
            sto_mutex_enter_ex();
            rcode= sto_flash_auto_sec_erase(dev, start_offset, \
              len_in_k);
            sto_mutex_exit_ex();
            return rcode ;
        case STO_DRIVER_SET_FLAG:
            dev->flag = param;
            return SUCCESS;

        case STO_DRIVER_SECTOR_BUFFER:
            dev->sector_buffer = (unsigned char *)param;
            return SUCCESS;
        case STO_DRIVER_GET_FLASH_INFO:
            sto_sflash_info(param);
            return SUCCESS;
    }

    /*private IO_Control of SPECIFIC sto device*/
    if(tp->io_ctrl)
    {
        return    tp->io_ctrl(cmd, param);
    }
    if(SUCCESS!=rcode)
    {
        return ERR_FLASH_NEED_ERASE;
    }
    return SUCCESS;
}

static int copy_pre_fix_block(struct flash_private *tp, unsigned char **p, unsigned char *oper_p, \
                                   unsigned long sector_start, unsigned long l)
{

    if (l != 0)
    {
        /* check if combination buffer existed */
        if (NULL == oper_p)
        {
            return ERR_FLASH_NO_BUFFER;
        }
        if(0 == remote_flash_read_tmo)
        {
            tp->read(*p, FLASH_OFF2PTR(sector_start), l);
        }
        else
        {
            if(SUCCESS!=(tp->read(*p, FLASH_OFF2PTR(sector_start), l)))
            {
                return ERR_FLASH_READ_FAIL;
            }
        }
        *p += l;
    }
    return SUCCESS;
}
static int sto_flash_change(struct sto_device *dev, unsigned long flag, \
    unsigned char *data, unsigned int len, unsigned char *buffer)
{
    int rcode = 0;
    unsigned char *p = NULL;
    unsigned char *oper_p = NULL;
    unsigned long oper_start = 0;
    unsigned long oper_end = 0;
    unsigned long oper_len = 0;
    unsigned long l = 0;
    unsigned long pre_fix_l = 0;
    unsigned long post_fix_l = 0;
    unsigned long target_start = 0;
    unsigned long target_end = 0;
    unsigned int sector_no = 0;
    unsigned long sector_start = 0;
    unsigned long sector_end = 0;
    unsigned long sector_size = 0;
    unsigned int ret = 0;
    struct flash_private *tp = (struct flash_private *)dev->priv;

    /* first of all, try directly write */
    target_start = flag & FLASH_ADDR_MASK;
#if 0//for sflash, the write operation will never compare the final content, it's dangerous.        
    MUTEX_ENTER();
    ret = tp->write(target_start, data, len);
    MUTEX_LEAVE();
    if (ret == 0)
    {
        return len;
    }
#endif
    /* check input data validation */
    if (target_start >= tp->flash_size)
    {
        return ERR_FLASH_OFFSET_ERROR;
    }
    /* limit input data to proper value */
    target_end = target_start + len;
    if (target_end > tp->flash_size)
    {
        target_end = tp->flash_size;
        len = target_end - target_start;
    }
    /* begin to write sector by sector */
    sector_no = sto_flash_sector_align(dev,target_start);
    sector_start = sto_flash_sector_start(dev,sector_no);
    while (target_start < target_end)
    {
        /* get information about current sector */
        sector_size = sto_flash_sector_size(dev,sector_no);
        sector_end = sector_start + sector_size;
        /* get information about current operation */
        oper_start = target_start > sector_start ? target_start : \
        sector_start;
        oper_end = target_end < sector_end ? target_end : \
        sector_end;
        oper_len = oper_end - oper_start;
        oper_p = data;
        pre_fix_l = 0;
        post_fix_l = 0;
        /* check flash operation */
        switch (tp->verify(oper_start, data, oper_len))
        {
            /* need erase */
            case 2:
                if (flag & STO_FLAG_AUTO_ERASE)
                {
                    /* check if need combine data */
                    if (oper_len != sector_size)
                    {
                        p = buffer;
                        oper_p = buffer;
                        /* copy pre-fix block from flash */
                        l = oper_start - sector_start;
                        pre_fix_l = l;
                        ret = copy_pre_fix_block (tp, &p, oper_p, sector_start, l);
                        if (ret != 0)
                        {
                            return ret;
                        }
  /*
                        if (l != 0)
                        {
                            ret = check_buf_existed (tp, p, oper_p, sector_start);
                            if (ret != 0)
                            {
                                return ret;
                            }
                            p += l;
                        }
    */
                        if (oper_p != NULL)
                        {
                            /* copy real data */
                            MEMCPY(p, data, oper_len);
                            p += oper_len;
                        }
                        if (flag & STO_FLAG_SAVE_REST)
                        {
                            /* copy post-fix block from flash */
                            l = sector_end - oper_end;
                            post_fix_l = l;
                            ret = copy_pre_fix_block (tp, &p, oper_p, oper_end, l);
                            if (ret != 0)
                            {
                                return ret;
                            }
                        }
                        /* update operation information */
                        if (NULL == oper_p)
                        {
                            oper_p = data;
                        }
                        else
                        {
                            oper_len = p - oper_p;
                        }
                        oper_start = sector_start;
                    }
                    /* erase sector */
                    //MUTEX_ENTER();
                    rcode = tp->erase_sector(sector_start);
                    //MUTEX_LEAVE();
                    if(0 == remote_flash_read_tmo)
                    {
                        if(SUCCESS!=rcode)
                        {
                            return ERR_FLASH_NEED_ERASE;
                        }
                    }
                    //MUTEX_ENTER();
                    ret = tp->write(oper_start, oper_p, oper_len);
                    //MUTEX_LEAVE();
                    if (ret != 0)
                    {
                        return ERR_FLASH_WRITE_FAIL;
                    }
                    /* verify the result */
                    ret = tp->verify(oper_start, oper_p, oper_len);
                    if (ret != 0)
                    {
                        return ERR_FLASH_WRITE_FAIL;
                    }
                    }
                    else
                    {
                        return ERR_FLASH_NEED_ERASE;
                    }
                    break;
            /* need write */
            case 1:
                //MUTEX_ENTER();
                ret = tp->write(oper_start, oper_p, oper_len);
                //MUTEX_LEAVE();
                if (ret != 0)
                {
                    return ERR_FLASH_WRITE_FAIL;
                }
                /* verify the result */
                ret = tp->verify(oper_start, oper_p, oper_len);
                if (ret != 0)
                {
                    return ERR_FLASH_WRITE_FAIL;
                }
                break;
            default:
                break;
        }
        /* advance to next data block (sector) */
        if(oper_len>=pre_fix_l)
        {
            oper_len -= pre_fix_l;
        }
        if(oper_len>=post_fix_l)
        {
            oper_len -= post_fix_l;
        }
        target_start += oper_len;
        data += oper_len;
        sector_start = sector_end;
        sector_no++;
    }
    return len;
}


/*
 *     Name        :   sto_flash_put_data()
 *    Description    :   Write data into storage
 *    Parameter    :    struct sto_device *dev        : Device
 *                    UINT32 offset                : Flash offset
 *                    UINT8 *data                    : Data to be write
 *                    UINT32 len                    : Data length
 *    Return        :    INT32                         : Write data length
 *
 */
static INT32 sto_flash_put_data(struct sto_device *dev, \
      UINT32 offset, UINT8 *data, INT32 len)
{
    INT32 ret = 0;
    unsigned long flag = 0;

    if (NULL == dev)
    {
        return ERR_PARA;
    }
    sto_mutex_enter_ex();
    flag = (dev->flag) | (STO_FLAG_AUTO_ERASE) | ((offset) & (FLASH_ADDR_MASK));
    ret= sto_flash_change(dev, flag, data, len, dev->sector_buffer);
    sto_mutex_exit_ex();
    return ret ;
}

/*
 *     Name        :   sto_flash_get_data()
 *    Description    :   Read data into storage
 *    Parameter    :    struct sto_device *dev        : Device
 *                    UINT8 *data                    : Data to be read
 *                    UINT32 offset                : Flash offset
 *                    UINT32 len                    : Data length
 *    Return        :    INT32                         : Read data length
 *
 */
static INT32 sto_flash_get_data(struct sto_device *dev, \
      UINT8 *data, UINT32 offset, INT32 len)
{
    struct flash_private *tp = NULL;
    int rcode = 0;

    if ((NULL == dev) || (NULL == data))
    {
        return ERR_PARA;
    }
    tp = (struct flash_private *)dev->priv;
    if ((INT32)offset + len > dev->totol_size)
    {
        if((UINT32)(dev->totol_size)>offset)
        {
            len = dev->totol_size - offset;
        }
        else
        {
            return ERR_FLASH_READ_FAIL;
        }
    }
    //MUTEX_ENTER();
    sto_mutex_enter_ex();
    rcode = tp->read(data, (UINT8 *)(dev->base_addr + offset), len);
    sto_mutex_exit_ex();
    //MUTEX_LEAVE();

    if(0 == remote_flash_read_tmo)
    {
        if(SUCCESS!=rcode)
        {
            return ERR_FLASH_READ_FAIL;
        }
    }/*REMOTE_FLASH_READ_TMO */

    return     len;
}


/*
 *     Name        :   sto_flash_write()
 *    Description    :   Write data into storage
 *    Parameter    :    struct sto_device *dev        : Device
 *                    UINT8 *data                    : Data to be write
 *                    UINT32 len                    : Data length
 *    Return        :    INT32                         : Write data length
 *
 */
static INT32 sto_flash_write(struct sto_device *dev, UINT8 *data, \
      INT32 len)
{
    INT32 ret = 0;
    int id = 0;

    if ((NULL == dev) || (NULL == data))
    {
        return ERR_PARA;
    }
    if (OSAL_INVALID_ID == (id = osal_task_get_current_id()))
    {
        id = 0;
    }
    if(id>=STO_TASK_SUPPORT_NUM_MAX)
    {
        //Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
        ASSERT(0);
        return ERR_FAILED;
    }
    sto_mutex_enter_ex();
    ret = sto_flash_change(dev, dev->flag | (dev->curr_addr[id] & \
          FLASH_ADDR_MASK), data, len, dev->sector_buffer);
    sto_mutex_exit_ex();
    if (ret > 0)
    {
        dev->curr_addr[id] += ret;
    }

    return ret;
}


/*
 * Read "len" number of data from current address point.
 */
static INT32 sto_flash_read(struct sto_device *dev, UINT8 *data, \
      INT32 len)
{
    int id = 0;
    int rcode = 0;
    INT32 len2 = len;
    UINT32 base_addr = 0;
    struct flash_private *tp = NULL;

    if ((NULL == dev) || (NULL == data))
    {
        return ERR_PARA;
    }
    base_addr = dev->base_addr;
    tp = (struct flash_private *)dev->priv;
    if (OSAL_INVALID_ID == (id = osal_task_get_current_id()))
    {
        id = 0;
    }
    if(id>=STO_TASK_SUPPORT_NUM_MAX)
    {
        //Need enlarge the value of STO_TASK_SUPPORT_NUM_MAX
        ASSERT(0);
        return ERR_FAILED;
    }
    if (dev->curr_addr[id] + len > dev->totol_size)
    {
        len = dev->totol_size - dev->curr_addr[id];
        len2 =len -1;
    }
    //MUTEX_ENTER();
    sto_mutex_enter_ex();
    rcode = tp->read(data,(UINT8 *)(base_addr+dev->curr_addr[id]),len);
    sto_mutex_exit_ex();
    //MUTEX_LEAVE();

    if(0 == remote_flash_read_tmo)
    {
        if(SUCCESS!=rcode)
        {
            return ERR_FLASH_READ_FAIL;
        }
    }/*REMOTE_FLASH_READ_TMO */

    dev->curr_addr[id] += len2;
    return len;
}

/**************************************************************
 * Function:
 *     sto_flash_sector_size()
 * Description:
 *     Return the size of specific sector.
 * Inpute:
 *    dev --- Device control block
 *    sector --- Sector number
 * Return Values:
 *    Size of the specific sector.
 ***************************************************************/
unsigned long sto_flash_sector_size(struct sto_device *dev,\
                unsigned int sector)
{
    unsigned char *p = NULL;
       struct flash_private *tp = (struct flash_private *)dev->priv;

    if (sector >= tp->flash_sectors)
    {
        sector = tp->flash_sectors - 1;
    }

    p = (unsigned char *)flash_sector_map + \
          flash_sector_begin[tp->flash_id] * 2;

    while (sector >= p[0])     {
        sector -= p[0];
        p += 2;
    }

    return 1 << p[1];
}

/**************************************************************
 * Function:
 *     sto_flash_sector_align()
 * Description:
 *     Return the sector which is align with the address.
 * Inpute:
 *    dev --- Device control block
 *    addr --- Specific address
 * Return Values:
 *    Sector which is align with the address.
 ***************************************************************/
unsigned int sto_flash_sector_align(struct sto_device *dev,\
                unsigned long addr)
{
    unsigned int i = 0;
    unsigned int sector_size = 0;
       struct flash_private *tp = (struct flash_private *)dev->priv;

    for (i = 0; i < tp->flash_sectors; i++)
    {
        sector_size = sto_flash_sector_size(dev,i);
        if (addr < sector_size)
        {
            break;
        }
        addr -= sector_size;
    }

    return i;
}

/**************************************************************
 * Function:
 *     sto_flash_sector_start()
 * Description:
 *     Return Start address of of specific sector.
 * Inpute:
 *    dev --- Device control block
 *    sector_no --- Sector number
 * Return Values:
 *    Start address of the specific sector.
 ***************************************************************/
unsigned long sto_flash_sector_start(struct sto_device *dev,\
                unsigned long sector_no)
{
    unsigned long addr = 0;

    while (sector_no--)
    {
        addr += sto_flash_sector_size(dev, sector_no);
    }

    return addr;
}

#ifdef FLASH_SOFTWARE_PROTECT
/**************************************************************
 * Function:
 * 	sto_flash_lock()
 * Description:
 * 	Lock flash by offset and length.
 * Inpute:
 *	Parameter	:
 *          struct sto_device *dev              : Device
 *          UINT32 offset                       : Data address
 *          UINT32 len                          : Data length
 * Return Values:
 *	Lock success with 0 or failed with -1.
 ***************************************************************/
static INT32 sto_flash_lock(struct sto_device *dev,INT32 offset, INT32 len)
{
	struct flash_private *tp = (struct flash_private *)dev->priv;
	INT32 ret = 0;

	osal_task_dispatch_off();
	if(tp->swp->swp_lock != NULL)
		ret=tp->swp->swp_lock(tp->swp, offset, len);
	osal_task_dispatch_on();

	return ret;
}

/**************************************************************
 * Function:
 * 	sto_flash_unlock()
 * Description:
 * 	Unock flash by offset and length.
 * Inpute:
 *	Parameter	:
 *          struct sto_device *dev              : Device
 *          UINT32 offset                       : Data address
 *          UINT32 len                          : Data length
 * Return Values:
 *	Unlock success with 0, and failed with -1.
 ***************************************************************/
static INT32 sto_flash_unlock(struct sto_device *dev,INT32 offset, INT32 len)
{
	struct flash_private *tp = (struct flash_private *)dev->priv;
	INT32 ret = 0;

	osal_task_dispatch_off();
	if(tp->swp->swp_unlock != NULL)
		ret=tp->swp->swp_unlock(tp->swp, offset, len);
	osal_task_dispatch_on();

	return ret;
}
/**************************************************************
 * Function:
 * 	sto_flash_get_lock_range()
 * Description:
 * 	Check flash lock or not by offset and length.
 * Inpute:
 *	Parameter	:
 *          struct sto_device *dev              : Device
 *          UINT32 *offset                      : Data locked address
 *          UINT32 *len                         : Data locked length
 * Return Values:
 *	Check success with 0, and failed with -1.
 ***************************************************************/
static INT32 sto_flash_get_lock_range(struct sto_device *dev,INT32 *offset, INT32 *len)
{
	struct flash_private *tp = (struct flash_private *)dev->priv;

	if(tp->swp->swp_get_lock_range != NULL)
		return tp->swp->swp_get_lock_range(tp->swp, offset, len);
	else
		return 0;
}
/**************************************************************
 * Function:
 * 	sto_flash_is_lock()
 * Description:
 * 	Check flash lock or not by offset and length.
 * Inpute:
 *	Parameter	:
 *          struct sto_device *dev              : Device
 *          UINT32 offset                       : Data address
 *          UINT32 len                          : Data length
 *          UINT32 *locked                      : Locked is 1, unlock is 0.
 * Return Values:
 *	Check success with 0, and failed with -1.
 ***************************************************************/
static INT32 sto_flash_is_lock(struct sto_device *dev,INT32 offset, INT32 len, INT32 *locked)
{
	struct flash_private *tp = (struct flash_private *)dev->priv;

//add by yuj 20170824
	*locked = 0;
	if(tp->swp->swp_is_lock != NULL)
		return tp->swp->swp_is_lock(tp->swp, offset, len, locked);
	else
		return 0;
}
#endif

/**************************************************************
 * Function:
 *     sto_fp_init()
 * Description:
 *     init common functions for device.
 * Inpute:
 *    None.
 * Return Values:
 *    None.
 ***************************************************************/
void sto_fp_init(struct sto_device *dev,INT32 (*init)(struct sto_flash_info *param))
{
    dev->init     = init;
    dev->open     = sto_flash_open;
    dev->close     = sto_flash_close;
    dev->do_ioctl     = sto_flash_ioctl;
    dev->lseek     = sto_flash_lseek;
    dev->write     = sto_flash_write;
    dev->put_data    = sto_flash_put_data;
    dev->get_data    = sto_flash_get_data;
    dev->read     = sto_flash_read;
#ifdef FLASH_SOFTWARE_PROTECT
    dev->lock       = sto_flash_lock;
    dev->unlock        = sto_flash_unlock;
    dev->is_lock        = sto_flash_is_lock;
    dev->get_lock_range = sto_flash_get_lock_range;
#endif
}

/**************************************************************
 * Function:
 *     sto_flash_soft_protect(unsigned long db_addr_low, unsigned long db_addr_up)
 * Description:
 *     Software protection of flash device except database whose space is defined
 * by unpro_addr_low and unpro_addr_up.
 * Inpute:
 *    db_addr_low --- Database address
 *    db_addr_up --- Database address
 * Return Values:
 *    None
 ***************************************************************/

void sto_flash_soft_protect(unsigned long db_addr_low, unsigned long db_addr_up)
{
    m_enable_soft_protection = TRUE;
    unpro_addr_low = SYS_FLASH_BASE_ADDR + db_addr_low;
    unpro_addr_up = SYS_FLASH_BASE_ADDR + db_addr_up;
    return;
}

void sto_flash_disable_protect(void)
{
    m_enable_soft_protection = FALSE;
}
/*
static unsigned long sto_flash_sector_erase(struct sto_device *dev, unsigned int sector)
{
       struct flash_private *tp = (struct flash_private *)dev->priv;

    return tp->erase_sector(sector);
}
*/
