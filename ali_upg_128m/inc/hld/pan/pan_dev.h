/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:    pan_dev.h
*
*    Description:    This file contains pan_device structure define in HLD.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*        1.Apr.21.2003      Justin Wu       Ver 0.1    Create file.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __HLD_PAN_DEV_H__
#define __HLD_PAN_DEV_H__
#ifdef __cplusplus
extern "C"
{
#endif
#include <types.h>
#include <hld/hld_dev.h>

/* IO control command */
enum pan_device_ioctrl_command
{
    PAN_DRIVER_ATTACH    = 0,        /* Front panel driver attach command */
    PAN_DRIVER_SUSPEND    = 1,        /* Front panel driver suspend command */
    PAN_DRIVER_RESUME    = 2,        /* Front panel driver resume command */
    PAN_DRIVER_DETACH    = 3,        /* Front panel driver detach command */
    PAN_DRIVER_READ_LNB_POWER = 4,    /* Front panel driver NIM LNB power protect status */
    PAN_DRIVER_WRITE_LED_ONOFF =5,
    PAN_DRIVER_UART_SELECT = 6,
    PAN_DRIVER_SET_GPIO    = 7,        /* Front panel driver set gpio command */
    PAN_DRIVER_GET_GPIO = 8,        /* Front panel driver get gpio value */
    PAN_DRIVER_STANDBY  = 9,        /* Front panel driver enter low power mode */
    PAN_DRIVER_SK_DETECT_POLAR = 10,/* Front panel driver set key detect polor, only enable for shadow scan */
};

enum pan_key_type
{
    PAN_KEY_TYPE_INVALID    = 0,    /* Invalid key type */
    PAN_KEY_TYPE_REMOTE        = 1,    /* Remote controller */
    PAN_KEY_TYPE_PANEL        = 2,    /* Front panel */
    PAN_KEY_TYPE_JOYSTICK    = 3,    /* Game joy stick */
    PAN_KEY_TYPE_KEYBOARD    = 4        /* Key board */
};

enum pan_key_press
{
    PAN_KEY_RELEASE        = 0,
    PAN_KEY_PRESSED        = 1
};

struct pan_device_stats
{
    UINT16    display_num;            /* Number of display data */
};

struct pan_key
{
    UINT8  type;                    /* The key type */
    UINT8  state;                    /* The key press state */
    UINT16 count;                    /* The key counter */
    UINT32 code;                    /* The value */
};

/* enum pan_ir_protocol */
typedef enum pan_ir_protocol
{
    IR_TYPE_NEC = 0,
    IR_TYPE_LAB,
    IR_TYPE_50560,
    IR_TYPE_KF,
    IR_TYPE_LOGIC,
    IR_TYPE_SRC,
    IR_TYPE_NSE,
    IR_TYPE_RC5,
    IR_TYPE_RC5_X,
    IR_TYPE_RC6,
    IR_TYPE_UNDEFINE
}pan_ir_protocol_e;


/* struct pan_ir_endian */
struct pan_ir_endian
{
    unsigned int enable;
    pan_ir_protocol_e protocol;
    unsigned int bit_msb_first;
    unsigned int byte_msb_first;
};
/*
 *  Structure pan_device, the basic structure between HLD and LLD of panel
 *    device.
 */

struct pan_device
{
    struct hld_device   *next;        /* Next device structure */
    UINT32        type;                        /* Interface hardware type */
    INT8        name[HLD_MAX_NAME_SIZE];    /* Device name */

    UINT16        flags;                /* Interface flags, status and ability */

    UINT16        led_num;            /* Number of LED */
    UINT16        key_num;            /* Number of input keys in Front panel */

    /* Hardware privative structure */
    void        *priv;                /* pointer to private data */

/*
 *  Functions of this panel device
 */
    INT32    (*init)();

    INT32    (*open)(struct pan_device *dev);

    INT32    (*stop)(struct pan_device *dev);

    void    (*display)(struct pan_device *dev, char *data, UINT32 len);

    INT32    (*send_data)(struct pan_device *dev, UINT8 *data, UINT32 len, UINT32 timeout);

    INT32    (*receive_data)(struct pan_device *dev, UINT8 *data, UINT32 len, UINT32 timeout);

    INT32    (*do_ioctl)(struct pan_device *dev, INT32 cmd, UINT32 param);

    struct pan_device_stats *(*get_stats)(struct pan_device *dev);
};
#ifdef __cplusplus
 }
#endif
#endif  /* __HLD_PAN_DEV_H__ */
