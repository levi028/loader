/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    pan.c
*
*    Description:    This file contains all functions definition
*                     of Front panel driver.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Apr.21.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>

#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>


#if((ALI_M3327C == SYS_CHIP_MODULE) && (2 == SYS_SDRAM_SIZE))
    #define PAN_RX_BUFF_SIZE        4
#else
    #define PAN_RX_BUFF_SIZE        8
#endif
static UINT8 pan_enable_repeat = 1;
static UINT8 pan_rx_buff_head = 0;
static UINT8 pan_rx_buff_tail = 0;
static struct pan_key    input_pan_key;                /* Current input key */
static UINT32 pan_rx_buff[PAN_RX_BUFF_SIZE][sizeof(struct pan_key)] = {{0,0}};
static struct pan_ir_endian g_ir_endian = {0, IR_TYPE_UNDEFINE, 0, 0};

/*
 *     Name        :   pan_open()
 *    Description    :   Open a pan device
 *    Parameter    :    struct pan_device *dev        : Device to be openned
 *    Return        :    INT32                         : Return value
 *
 */
INT32 pan_open(struct pan_device *dev)
{
    INT32 result = RET_FAILURE;

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }

    /* If openned already, exit */
    if (dev->flags & HLD_DEV_STATS_UP)
    {
        PRINTF("pan_open: warning - device %s openned already!\n", dev->name);
        return SUCCESS;
    }
    result = pan_buff_clear();

    /* Open this device */
    if (dev->open)
    {
        result = dev->open(dev);
    }

    /* Setup init work mode */
    if (SUCCESS == result)
    {
        dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    }

    return result;
}

/*
 *     Name        :   pan_close()
 *    Description    :   Close a pan device
 *    Parameter    :    struct pan_device *dev        : Device to be closed
 *    Return        :    INT32                         : Return value
 *
 */
INT32 pan_close(struct pan_device *dev)
{
    INT32 result = RET_FAILURE;

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }

    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        PRINTF("pan_close: warning - device %s closed already!\n", dev->name);
        return SUCCESS;
    }

    /* Stop device */
    if (dev->stop)
    {
        result = dev->stop(dev);
    }

    /* Update flags */
    dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    return result;
}

/*
 *     Name        :   pan_get_key()
 *    Description    :   Get a input code
 *    Parameter    :    struct pan_device *dev        : Device get code from
 *                    UINT32 timeout                : Timeout mode
 *    Return        :    UINT32                         : Return code
 *
 */
struct pan_key *pan_get_key(struct pan_device *dev, UINT32 timeout)
{
    while (1)
    {
        if (pan_rx_buff_head != pan_rx_buff_tail)
        {
            break;
        }
        if (timeout != OSAL_WAIT_FOREVER_TIME)
        {
            if (0 == timeout--)
            {
                return NULL;
            }
        }
        osal_task_sleep(1);
    }

    MEMCPY(&input_pan_key, pan_rx_buff[pan_rx_buff_tail], sizeof(struct pan_key));
    pan_rx_buff_tail++;
    pan_rx_buff_tail %= PAN_RX_BUFF_SIZE;

    return &input_pan_key;
}

INT32 pan_send_data(struct pan_device *dev, UINT8 *data, UINT32 len, UINT32 timeout)
{
    INT32 result = RET_FAILURE;

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }

    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        PRINTF("pan_send_data: warning - device %s closed already!\n", dev->name);
        return SUCCESS;
    }

    /* Send data */
    if (dev->send_data)
    {
        result = dev->send_data(dev, data, len, timeout);
    }

    /* Update flags */
    dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    return result;
}

INT32 pan_receive_data(struct pan_device *dev, UINT8 *data, UINT32 len, UINT32 timeout)
{
    INT32 result = RET_FAILURE;

    if (NULL == dev)
    {
        return RET_SUCCESS;
    }


    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        PRINTF("pan_receive_data: warning - device %s closed already!\n", dev->name);
        return SUCCESS;
    }

    /* Receive data */
    if (dev->receive_data)
    {
        result = dev->receive_data(dev, data, len, timeout);
    }

    /* Update flags */
    dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

    return result;
}


/*
 *     Name        :   pan_buff_clear()
 *    Description    :   Clear key buffer.
 *    Parameter    :
 *    Return        :
 *
 */
INT32 pan_buff_clear(void)
{
    osal_interrupt_disable();
    pan_rx_buff_head = 0;
    pan_rx_buff_tail = 0;
    osal_interrupt_enable();
    return SUCCESS;

}

/*
 *     Name        :   pan_buff_queue_tail()
 *    Description    :   Add a code to list tail.
 *    Parameter    :
 *    Return        :
 *
 */
INT32 pan_buff_queue_tail(struct pan_key *key)
{
    if(NULL != key)
    {
        osal_interrupt_disable();
        MEMCPY(pan_rx_buff[pan_rx_buff_head], key, sizeof(struct pan_key));
        pan_rx_buff_head++;
        pan_rx_buff_head %= PAN_RX_BUFF_SIZE;
        osal_interrupt_enable();

        return SUCCESS;
    }
    return RET_FAILURE;
}

#if((SYS_PROJECT_FE == PROJECT_FE_DVBT) && (2 == SYS_SDRAM_SIZE))
__ATTRIBUTE_REUSE_
#endif
/*
static void pan_buff_set_repeat(UINT8 enable_repeat)
{
    pan_enable_repeat = enable_repeat;
}
*/
UINT8 pan_buff_get_repeat(void)
{
    return pan_enable_repeat;
}

/*
 *     Name        :   pan_display()
 *    Description    :   Show information in LED
 *    Parameter    :    struct pan_device *dev        : Device get code from
 *                    char *data                    : Data point to be show
 *                    UINT32 len                    : Data length
 *    Return        :
 *
 */
INT32 pan_display(struct pan_device *dev, char *data, UINT32 len)
{
    /* If device not running, exit */
    if (NULL == dev)
    {
        return ERR_DEV_ERROR;
    }

#if defined(FPGA_TEST) || (SYS_MAIN_BOARD == BOARD_SB_S3602F_QFP_01V01)
    return RET_SUCCESS;
#endif

    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->display)
    {
        dev->display(dev, data, len);
    }

    return SUCCESS;
}

/*
 *     Name        :   pan_io_control()
 *    Description    :   Panel IO control command
 *    Parameter    :    struct sto_device *dev        : Device
 *                    INT32 cmd                    : IO command
 *                    UINT32 param                : Param
 *    Return        :    INT32                         : Result
 *
 */
INT32 pan_io_control(struct pan_device *dev, INT32 cmd, UINT32 param)
{
    if (NULL == dev)
    {
        return RET_SUCCESS;
    }

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }

    if (dev->do_ioctl)
    {
        return dev->do_ioctl(dev, cmd, param);
    }

    return SUCCESS;
}

void pan_ir_set_endian(struct pan_ir_endian ir_endian)
{
    g_ir_endian.enable = ir_endian.enable;
    g_ir_endian.protocol= ir_endian.protocol;
    g_ir_endian.bit_msb_first = ir_endian.bit_msb_first;
    g_ir_endian.byte_msb_first = ir_endian.byte_msb_first;
}

struct pan_ir_endian *pan_ir_get_endian(void)
{
    return &g_ir_endian;
}
